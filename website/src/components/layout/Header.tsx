import { Link } from 'react-router-dom';

const NAV_LINKS = [
  { label: 'Platform', to: '/platform' },
  { label: 'Products', to: '/products' },
  { label: 'Resources', to: '/resources' },
  { label: 'Careers', to: '/careers' },
  { label: 'Contact', to: '/contact' },
];

export function Header() {
  return (
    <header style={{
      position: 'sticky',
      top: 0,
      zIndex: 100,
      backgroundColor: 'rgba(12,14,18,0.85)',
      backdropFilter: 'blur(12px)',
      borderBottom: '1px solid var(--color-border-subtle)',
    }}>
      <nav style={{
        maxWidth: '1200px',
        margin: '0 auto',
        padding: '0 1.5rem',
        height: '56px',
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'space-between',
      }}>
        <Link to="/" style={{
          textDecoration: 'none',
          display: 'flex',
          alignItems: 'center',
          gap: '0.5rem',
        }}>
          <span style={{ fontSize: '1.2rem' }}>⚡</span>
          <span style={{
            fontSize: '0.95rem',
            fontWeight: 700,
            color: 'var(--color-text-primary)',
            letterSpacing: '-0.02em',
          }}>NEPA</span>
        </Link>
        <div style={{ display: 'flex', alignItems: 'center', gap: '1.5rem' }}>
          {NAV_LINKS.map((link) => (
            <Link key={link.to} to={link.to} style={{
              textDecoration: 'none',
              fontSize: '0.8rem',
              fontWeight: 500,
              color: 'var(--color-text-secondary)',
            }}>
              {link.label}
            </Link>
          ))}
        </div>
      </nav>
    </header>
  );
}
