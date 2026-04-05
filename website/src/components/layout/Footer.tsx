import { Link } from 'react-router-dom';
import { footerNavSections } from '../../content/navigation';

export function Footer() {
  return (
    <footer className="border-t border-border-default bg-bg-primary">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-16">
        <div className="grid grid-cols-2 md:grid-cols-4 lg:grid-cols-5 gap-8 mb-12">
          {/* Brand */}
          <div className="col-span-2 md:col-span-4 lg:col-span-1 mb-4 lg:mb-0">
            <Link to="/" className="text-text-primary font-semibold text-lg tracking-tight">
              <span className="text-accent-primary font-mono text-sm mr-1">▲</span>
              AURASENSE
            </Link>
            <p className="mt-3 text-sm text-text-muted leading-relaxed max-w-xs">
              Neuromorphic edge intelligence for inspection infrastructure.
            </p>
            <p className="mt-4 text-xs text-text-muted">
              support@aurasensehk.com
            </p>
          </div>

          {/* Nav Sections */}
          {footerNavSections.map((section) => (
            <div key={section.title}>
              <h3 className="text-xs font-mono font-medium tracking-widest uppercase text-text-muted mb-4">
                {section.title}
              </h3>
              <ul className="space-y-2.5">
                {section.items.map((item) => (
                  <li key={item.href}>
                    <Link
                      to={item.href}
                      className="text-sm text-text-secondary hover:text-text-primary transition-colors"
                    >
                      {item.label}
                    </Link>
                  </li>
                ))}
              </ul>
            </div>
          ))}
        </div>

        <div className="border-t border-border-default pt-8 flex flex-col sm:flex-row justify-between items-center gap-4">
          <p className="text-xs text-text-muted">
            © {new Date().getFullYear()} AuraSense Limited. Hong Kong SAR. All rights reserved.
          </p>
          <p className="text-xs text-text-muted">
            NEPA · SFSVC · NERMN · NSSIM
          </p>
        </div>
      </div>
    </footer>
  );
}
