import { Link } from 'react-router-dom';
import { footerColumns, supportEmail } from '../../content/footer';

export function Footer() {
  return (
    <footer className="border-t border-border-subtle bg-surface">
      <div className="max-w-7xl mx-auto px-6 py-14">
        {/* Columns */}
        <div className="grid grid-cols-2 md:grid-cols-3 lg:grid-cols-5 gap-10">
          {footerColumns.map((col) => (
            <div key={col.title}>
              <h4 className="text-text-primary text-sm font-semibold mb-4">
                {col.title}
              </h4>
              <ul className="space-y-2">
                {col.links.map((link) => (
                  <li key={link.href}>
                    <Link
                      to={link.href}
                      className="text-text-muted text-sm hover:text-text-secondary transition-colors duration-150"
                    >
                      {link.label}
                    </Link>
                  </li>
                ))}
              </ul>
            </div>
          ))}
        </div>

        {/* Bottom bar */}
        <div className="mt-12 pt-8 border-t border-border-subtle flex flex-col sm:flex-row items-center justify-between gap-4">
          <p className="text-text-muted text-xs">
            &copy; 2025 AuraSense. All rights reserved.
          </p>
          <a
            href={`mailto:${supportEmail}`}
            className="text-text-muted text-xs hover:text-text-secondary transition-colors duration-150"
          >
            {supportEmail}
          </a>
        </div>
      </div>
    </footer>
  );
}
