import { useState, useEffect, useRef } from 'react';
import { Link, useLocation } from 'react-router-dom';
import { mainNavLinks } from '../../content/navigation';

export function Header() {
  const [mobileOpen, setMobileOpen] = useState(false);
  const [openDropdown, setOpenDropdown] = useState<string | null>(null);
  const navRef = useRef<HTMLElement>(null);
  const location = useLocation();

  function closeMenus() {
    setMobileOpen(false);
    setOpenDropdown(null);
  }

  // Close dropdown on outside click
  useEffect(() => {
    function handleClick(e: MouseEvent) {
      if (
        navRef.current &&
        !navRef.current.contains(e.target as Node)
      ) {
        setOpenDropdown(null);
      }
    }
    document.addEventListener('mousedown', handleClick);
    return () => document.removeEventListener('mousedown', handleClick);
  }, []);

  function isActive(href: string) {
    return location.pathname === href || location.pathname.startsWith(href + '/');
  }

  return (
    <header className="fixed top-0 inset-x-0 z-50 bg-surface/80 backdrop-blur-xl border-b border-border-subtle">
      <div className="max-w-7xl mx-auto px-6 h-16 flex items-center justify-between">
        {/* Logo */}
        <Link to="/" onClick={closeMenus} className="nav-logo-link">
          <img src="/assets/logo-aurasensehk.svg" alt="AuraSenseHK" className="nav-logo-img" />
          <span className="nav-logo-text">AuraSense</span>
        </Link>

        {/* Desktop nav */}
        <nav ref={navRef} className="hidden md:flex items-center gap-8">
          {mainNavLinks.map((link) =>
            link.children ? (
              <div key={link.label} className="relative">
                <button
                  type="button"
                  onClick={() => setOpenDropdown((o) => o === link.label ? null : link.label)}
                  className={`flex items-center gap-1 text-sm transition-colors duration-150 ${
                    isActive(link.href)
                      ? 'text-accent'
                      : 'text-text-secondary hover:text-text-primary'
                  }`}
                >
                  {link.label}
                  <svg
                    className={`w-3.5 h-3.5 transition-transform duration-150 ${openDropdown === link.label ? 'rotate-180' : ''}`}
                    fill="none"
                    viewBox="0 0 24 24"
                    stroke="currentColor"
                    strokeWidth={2}
                  >
                    <path strokeLinecap="round" strokeLinejoin="round" d="M19 9l-7 7-7-7" />
                  </svg>
                </button>

                {openDropdown === link.label && (
                  <div className="absolute top-full left-0 mt-2 w-48 bg-surface-raised border border-border-subtle rounded-lg py-2 shadow-lg">
                    {link.children.map((child) => (
                      <Link
                        key={child.href}
                        to={child.href}
                        onClick={closeMenus}
                        className={`block px-4 py-2 text-sm transition-colors duration-150 ${
                          isActive(child.href)
                            ? 'text-accent'
                            : 'text-text-secondary hover:text-text-primary hover:bg-surface-overlay'
                        }`}
                      >
                        {child.label}
                      </Link>
                    ))}
                  </div>
                )}
              </div>
            ) : (
              <Link
                key={link.href}
                to={link.href}
                onClick={closeMenus}
                className={`text-sm transition-colors duration-150 ${
                  isActive(link.href)
                    ? 'text-accent'
                    : 'text-text-secondary hover:text-text-primary'
                }`}
              >
                {link.label}
              </Link>
            ),
          )}
        </nav>

        {/* Mobile hamburger */}
        <button
          type="button"
          onClick={() => setMobileOpen((o) => !o)}
          className="md:hidden text-text-secondary hover:text-text-primary"
          aria-label="Toggle navigation"
        >
          {mobileOpen ? (
            <svg className="w-6 h-6" fill="none" viewBox="0 0 24 24" stroke="currentColor" strokeWidth={2}>
              <path strokeLinecap="round" strokeLinejoin="round" d="M6 18L18 6M6 6l12 12" />
            </svg>
          ) : (
            <svg className="w-6 h-6" fill="none" viewBox="0 0 24 24" stroke="currentColor" strokeWidth={2}>
              <path strokeLinecap="round" strokeLinejoin="round" d="M4 6h16M4 12h16M4 18h16" />
            </svg>
          )}
        </button>
      </div>

      {/* Mobile drawer */}
      {mobileOpen && (
        <nav className="md:hidden border-t border-border-subtle bg-surface-raised px-6 py-4 space-y-1">
          {mainNavLinks.map((link) => (
            <div key={link.label}>
              <Link
                to={link.href}
                onClick={closeMenus}
                className={`block py-2 text-sm transition-colors duration-150 ${
                  isActive(link.href)
                    ? 'text-accent'
                    : 'text-text-secondary hover:text-text-primary'
                }`}
              >
                {link.label}
              </Link>
              {link.children?.map((child) => (
                <Link
                  key={child.href}
                  to={child.href}
                  onClick={closeMenus}
                  className={`block py-2 pl-4 text-sm transition-colors duration-150 ${
                    isActive(child.href)
                      ? 'text-accent'
                      : 'text-text-muted hover:text-text-primary'
                  }`}
                >
                  {child.label}
                </Link>
              ))}
            </div>
          ))}
        </nav>
      )}
    </header>
  );
}
